#!/bin/bash
# This script generates a sample security log file for the challenge task.

NUM_LINES=100  # Adjust for file size
OUTPUT_FILE="security_log.txt"

function generate_ip() {
  ip=$(printf "%d.%d.%d.%d" $(shuf -i 1-254 -n 4))
  echo "$ip"
}

for i in $(seq 1 $NUM_LINES); do
  timestamp=$(date +%Y-%m-%d\ %H:%M:%S -d "$i seconds ago")
  severity=$(shuf -e INFO WARNING ERROR ALERT -n 1)
  source_ip=$(generate_ip)
  dest_ip=$(generate_ip)
  event_type=$(shuf -e LOGIN_ATTEMPT FILE_ACCESS NETWORK_CONNECTION INTRUSION_DETECTED -n 1)
  case "$event_type" in
    LOGIN_ATTEMPT)
      message="Login attempt from $source_ip to $dest_ip. Username: user$((RANDOM % 100))"
      ;;
    FILE_ACCESS)
      message="File access from $source_ip to $dest_ip. File: /etc/passwd"
      ;;
    NETWORK_CONNECTION)
      message="Network connection established between $source_ip and $dest_ip. Port: $((RANDOM % 65535))"
      ;;
    INTRUSION_DETECTED)
      message="Possible intrusion detected from $source_ip. Suspicious activity on $dest_ip"
      ;;
    *)
      message="Unknown event type"
      ;;
  esac
  echo "$timestamp $severity $source_ip $dest_ip $event_type $message" >> "$OUTPUT_FILE"
done