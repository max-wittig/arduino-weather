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


def translate(forecast: str) -> str:
    # https://openweathermap.org/weather-conditions
    forecast = forecast.lower()
    forecast = forecast.replace(
        "thunderstorm with light rain", "Gewitter+leichter Regen"
    )
    forecast = forecast.replace("thunderstorm with rain", "Gewitter+Regen")
    forecast = forecast.replace(
        "thunderstorm with heavy rain", "Gewitter+starker Regen"
    )
    forecast = forecast.replace("light thunderstorm", "leichtes Gewitter")
    forecast = forecast.replace("broken clouds", "vereinzelt Wolken")
    forecast = forecast.replace("clear sky", "Klarer Himmel")
    forecast = forecast.replace("overcast clouds", "bedeckt")

    forecast = forecast.replace("thunderstorm", "Gewitter")
    forecast = forecast.replace("with", "mit")
    forecast = forecast.replace("rain", "Regen")
    forecast = forecast.replace("light", "leicht")
    forecast = forecast.replace("heavy", "heftiger")
    forecast = forecast.replace("drizzle", "tropfen")
    forecast = forecast.replace("ragged", "stuermig")
    forecast = forecast.replace("intensity", "Intensitaet")
    forecast = forecast.replace("shower", "starker")
    forecast = forecast.replace("moderate", "mittlerer")
    forecast = forecast.replace("very", "sehr")
    forecast = forecast.replace("extreme", "extrem")
    forecast = forecast.replace("freezing", "gefrorener")
    forecast = forecast.replace("snow", "Schnee")
    forecast = forecast.replace("sleet", "Graupel")
    forecast = forecast.replace("sky", "Himmel")
    forecast = forecast.replace("clouds", "Wolken")
    forecast = forecast.replace("broken", "gebrochene")
    forecast = forecast.replace("few", "wenige")
    forecast = forecast.replace("scattered", "einzelne")
    forecast = forecast.replace("overcast", "ueberzogen")
    forecast = forecast.replace("squalls", "Stuermboehen")
    forecast = forecast.replace("dust", "Staub")
    forecast = forecast.replace("sand", "Sand")
    forecast = forecast.replace("fog", "Nebel")
    forecast = forecast.replace("haze", "Dunst")
    forecast = forecast.replace("smoke", "Rauch")
    return forecast


def get_forecast(location: str) -> str:
    forecast = client.three_hours_forecast(location).get_forecast()
    if not forecast:
        return ""
    result = ""
    for cast in forecast:
        return translate(cast.get_detailed_status())


def get_weather() -> Dict[str, str]:
    results = []
    for location in locations.split(";"):
        temperature = (
            client.weather_at_place(location).get_weather().get_temperature("celsius")
        )
        city = location.split(",")[0][:7]
        results.append(
            {
                "city": city,
                "temp": str(temperature["temp"]),
                "forecast": get_forecast(location)[:16],
            }
        )
    logger.info("Got weather update")
    return results


def main():
    refresh_time = 15 * 60
    if dry:
        lcd = None
        logger.info("LCD initialized")
    else:
        lcd = serial.Serial(serial_device, 9600, timeout=0)
        time.sleep(5)
    while True:
        weather = json.dumps(get_weather())
        logger.info(f"Write {weather} to LCD")
        if not dry:
            lcd.write(bytes(weather, "utf-8"))
        logger.info(f"Sleeping for {refresh_time}")
        time.sleep(refresh_time)


if __name__ == "__main__":
    main()
