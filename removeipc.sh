#!/bin/bash
ipcs -m | grep gabrielebuoso | awk '{ print $2 }' | xargs -n1 ipcrm -m
ipcs -s | grep gabrielebuoso | awk '{ print $2 }' | xargs -n1 ipcrm -s
ipcs -q | grep gabrielebuoso | awk '{ print $2 }' | xargs -n1 ipcrm -q