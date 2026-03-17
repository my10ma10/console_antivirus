#!/bin/bash
set -e 
mkdir -p coverage

lcov --capture --directory . --output-file coverage/coverage.info
lcov --remove coverage/coverage.info \
    '*/external/*' \
    '*/tests/*' \
    '*/usr/*' \
    '*/build/*' \
    -o coverage/coverage_filtered.info

cp coverage/coverage_filtered.info coverage/coverage_final.info

genhtml coverage/coverage_final.info --output-directory coverage/html
