arduino-cli.exe compile -b arduino:avr:uno .
arduino-cli.exe upload -p COM4 -b arduino:avr:uno .

baudrate="${2:-9600}"

if  [[ "$1" == "-d" ]]; then
    arduino-cli.exe monitor -p COM4 -c baudrate=$baudrate | grep --text "debug"
else 
    arduino-cli.exe monitor -p COM4 -c baudrate=$baudrate
fi