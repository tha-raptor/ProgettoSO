#!/bin/bash
ipcs -m | grep andrea | awk '{ print $2 }' | xargs -n1 ipcrm -m
ipcs -s | grep andrea | awk '{ print $2 }' | xargs -n1 ipcrm -s
ipcs -q | grep andrea | awk '{ print $2 }' | xargs -n1 ipcrm -q

