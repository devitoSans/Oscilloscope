# User will run from this script.

# Dynamic change: python -> Serial port -> Arduino
# Static change (i.e. baudrate, batch num, etc): python -> file to state restart -> this script -> run over again

# My points: Bash scripts are the one that connects between program (C, python, etc)

arduino-cli.exe compile -b arduino:avr:uno .
arduino-cli.exe upload -p COM4 -b arduino:avr:uno .

# baudrate="${2:-115200}"

# if  [[ "$1" == "-d" ]]; then
#     arduino-cli.exe monitor -p COM4 -c baudrate=$baudrate | grep --text "debug"
# else 
#     arduino-cli.exe monitor -p COM4 -c baudrate=$baudrate
# fi

sleep 1

py main.py