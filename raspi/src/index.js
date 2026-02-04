require("dotenv").config();
const { SerialPort } = require("serialport");
const moment = require("moment");

const port = new SerialPort({
  path: process.env.SERIAL_PATH || "/dev/ttyUSB0",
  baudRate: process.env.SERIAL_BAUD || 115200,
});

// serial open callback
port.open((err) => {
  //
  //if (err) console.error("err (?): " + err.message);

  // serial data received callback
  port.on("data", (data) => {
    const m = moment();
    //const ts = m.utc().toISOString();

    // unix ms timestamps
    const ts = m.valueOf();

    const valStr = data?.toString().split(",");
    let values = {};

    // parse values to object
    let i = 1;
    for (const s of valStr) {
      values["L" + i] = parseFloat(s);
      i++;
    }

    //const obj = { ts, values };
    //console.log(obj);

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

    console.log(
      "tuova-iot,sensor_id=ESP32 " +
        Object.entries(values)
          .map(([key, val]) => key + "=" + val)
          .join(",") +
        " " +
        ts,
    );
  });
});

// repeat every second
setInterval(() => {
  // write measurement command
  port.write("m\n");
}, 1000);
