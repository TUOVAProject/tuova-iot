require("dotenv").config();
const { SerialPort } = require("serialport");
const moment = require("moment");

// configure serial port
const serialParams = {
    path: process.env.SERIAL_PATH || "/dev/ttyUSB0",
    baudRate: parseInt(process.env.SERIAL_BAUD) || 115200,
};
let port = new SerialPort(serialParams);

// serial port open callback:
const openCallback = (err) => {
    // serial data received callback:
    port.on("data", (data) => {
        // create timestamp as unix milliseconds
        const m = moment();
        const ts = m.valueOf();

        // split received values to array
        const arr = data?.toString().split(",");

        // create key-value pairs
        let values = {};
        let i = 1;
        for (const s of arr) {
            values["L" + i] = parseFloat(s);
            i++;
        }

        // post to influxdb
        fetch(
            process.env.INFLUXDB_API_BASE_URL +
                "/api/v2/write?org=" +
                process.env.INFLUXDB_API_ORG +
                "&bucket=" +
                process.env.INFLUXDB_API_BUCKET +
                "&precision=ms",
            {
                method: "POST",
                headers: {
                    "Content-Type": "text/plain; charset=utf-8",
                    Authorization: "Token " + process.env.INFLUXDB_API_TOKEN,
                },
                body:
                    (process.env.INFLUXDB_MEASUREMENT_NAME || "tuova-iot") +
                    ",sensor_id=" +
                    (process.env.INFLUXDB_SENSOR_ID || "ESP32") +
                    " " +
                    Object.entries(values)
                        .map(([key, val]) => key + "=" + val)
                        .join(",") +
                    " " +
                    ts,
            },
        );
    });
};

// open serial port with the callback
port.open(openCallback);

// serial port reconnect thing
// check every 5 seconds and re-create the serial connection if needed
setInterval(() => {
    if (!port.isOpen) {
        log("serial reconnect... ");
        port = new SerialPort(serialParams);
        port.open(openCallback);
    }
}, 5000);

// write the measurement command "m" every second
setInterval(() => {
    port.write("m\n");
}, 1000);
