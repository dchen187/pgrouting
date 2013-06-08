#!/bin/bash
# ------------------------------------------------------------------------------
# Travis CI scripts 
# Copyright(c) pgRouting Contributors
#
# Build pgRouting
# Add PostgreSQL templates
# ------------------------------------------------------------------------------

cmake -DWITH_DD=ON 
make
sudo make install

# ------------------------------------------------------------------------------
# PostgreSQL settings and templates
# ------------------------------------------------------------------------------
DBUSER="postgres"
DBNAME="pgrouting"

POSTGRESQL_VERSION="$1"
POSTGIS_VERSION="$2"

# Define alias function for psql command
run_psql () {
    PGOPTIONS='--client-min-messages=warning' psql -X -q -v ON_ERROR_STOP=1 --pset pager=off "$@"
}

# ------------------------------------------------------------------------------
# Set PostgreSQL users and permissions
# ------------------------------------------------------------------------------
sudo cp $TRAVIS_BUILD_DIR/tools/travis/pg_hba.conf `find /etc/postgresql/*/main/pg_hba.conf`
sudo /etc/init.d/postgresql restart	

# Disable password (better: create new user)
sudo -u $DBUSER psql -c "ALTER ROLE postgres WITH PASSWORD '';"

# ------------------------------------------------------------------------------
# Create database templates
# ------------------------------------------------------------------------------

# PostGIS
# ------------------------------------------------------------------------------
run_psql -U $DBUSER -c "CREATE DATABASE template_postgis ENCODING 'UTF8' TEMPLATE template0;"

if [ "$POSTGRESQL_VERSION" == "8.4" ] 
then
    run_psql -U $DBUSER -d template_postgis -c "CREATE LANGUAGE plpgsql;"
fi

run_psql -U $DBUSER -d template_postgis -f `find /usr/share -name "postgis.sql"`
run_psql -U $DBUSER -d template_postgis -f `find /usr/share -name "spatial_ref_sys.sql"`

run_psql -U $DBUSER -d template_postgis -c "GRANT ALL ON geometry_columns TO PUBLIC;"
run_psql -U $DBUSER -d template_postgis -c "GRANT ALL ON geography_columns TO PUBLIC;"
run_psql -U $DBUSER -d template_postgis -c "GRANT ALL ON spatial_ref_sys TO PUBLIC;"

run_psql -U $DBUSER -d template_postgis -c "VACUUM FULL;"
run_psql -U $DBUSER -d template_postgis -c "VACUUM FREEZE;"

run_psql -U $DBUSER -c "UPDATE pg_database SET datistemplate='true' WHERE datname='template_postgis';"
run_psql -U $DBUSER -c "UPDATE pg_database SET datallowconn='false' WHERE datname='template_postgis';"

# pgRouting
# ------------------------------------------------------------------------------
run_psql -U $DBUSER -c "CREATE DATABASE template_routing ENCODING 'UTF8' TEMPLATE template_postgis;"
run_psql -U $DBUSER -d template_routing -f `find /usr/share -name "pgrouting--*.sql"`

run_psql -U $DBUSER -d template_routing -c "VACUUM FULL;"
run_psql -U $DBUSER -d template_routing -c "VACUUM FREEZE;"

run_psql -U $DBUSER -c "UPDATE pg_database SET datistemplate='true' WHERE datname='template_routing';"
run_psql -U $DBUSER -c "UPDATE pg_database SET datallowconn='false' WHERE datname='template_routing';"
