import serial
import time
import datetime
import os
import json
import sys
import threading
import distutils.util
import pytz
import pyowm

from typing import Dict
from loguru import logger


timezone = os.getenv("TIMEZONE", "Europe/Zurich")
tz = pytz.timezone(timezone)
locations = os.getenv("LOCATIONS") or sys.exit(
    "LOCATIONS are required."
)  # e.g. London,GB;Zurich,CH;Munich,DE
api_key = os.getenv("OWM_API_KEY") or sys.exit("OWM_API_KEY is required.")
serial_device = os.getenv("SERIAL_DEVICE", "/dev/cu.usbmodem14301")
dry = distutils.util.strtobool(os.getenv("DRY_MODE", "False"))
client = pyowm.OWM(api_key)


def get_weather() -> Dict[str, str]:
    result = {"date": str(datetime.datetime.now(tz))[:16]}
    for location in locations.split(";"):
        temperature = (
            client.weather_at_place(location).get_weather().get_temperature("celsius")
        )
        city = location.split(",")[0][:7]
        result[city] = str(temperature["temp"])
    logger.info("Got weather update")
    return result


def main():
    refresh_time = 15 * 60
    if dry:
        lcd = None
        logger.info("LCD initialized")
    else:
        lcd = serial.Serial(serial_device, 9600, timeout=0)
        time.sleep(5)
    while True:
        weather = get_weather()
        json_string = json.dumps(weather)
        if dry:
            logger.info(f"Write {json_string} to LCD")
        else:
            lcd.write(bytes(json_string, "utf-8"))
        time.sleep(refresh_time)


if __name__ == "__main__":
    main()
