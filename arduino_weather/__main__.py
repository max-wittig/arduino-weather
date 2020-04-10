import serial
import time
import datetime
import os
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


class StoppableThread(threading.Thread):
    def __init__(self, *args, **kwargs):
        self.running = True
        super().__init__(*args, **kwargs)

    def stop(self):
        self.running = False


def get_weather() -> Dict[str, str]:
    result = {}
    for location in locations.split(";"):
        temperature = (
            client.weather_at_place(location).get_weather().get_temperature("celsius")
        )
        city = location.split(",")[0][:7]
        result[city] = temperature["temp"]
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
        thread: StoppableThread = LEDUpdateTask(args=(lcd, weather,), daemon=True)
        thread.start()
        logger.info("Started thread")
        time.sleep(refresh_time)
        thread.stop()
        thread.join()


class LEDUpdateTask(StoppableThread):
    def __init__(self, *args, **kwargs):
        self.lcd = kwargs.get("args")[0]
        self.weather = kwargs.get("args")[1]
        super(LEDUpdateTask, self).__init__(*args, **kwargs)

    def run(self):
        while self.running:
            self.update_locations()

    def update_locations(self):
        places_change_time = 3
        for location, temperature in self.weather.items():
            lcd_text = f"{location} {temperature} C"
            lcd_text_length = len(lcd_text)
            if lcd_text_length < 16:
                for i in range(lcd_text_length, 16):
                    lcd_text += " "
            lcd_text += str(datetime.datetime.now(tz))
            logger.info(f"Send {lcd_text}")
            if dry:
                logger.info("LCD text send")
            else:
                self.lcd.write(bytes(lcd_text, "utf-8"))
            time.sleep(places_change_time)


if __name__ == "__main__":
    main()
