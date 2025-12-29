from fetcher_config.definition import *

import serial
from abc import ABC, abstractmethod
from time import sleep

class SerialWrapper(ABC):
  @abstractmethod
  def read(self) -> str:
    pass

  @abstractmethod
  def write(self, data: str) -> None:
    pass

  @abstractmethod
  def write_bytes(self, data: int) -> None:
    pass

class PySerial(SerialWrapper):
  def __init__(self, config: Config):
    super().__init__()
    self._ser = serial.Serial(config.get("port"), config.get("baudrate"), timeout=config.get("timeout"))
    sleep(2)

  def read(self) -> str:
    return self._ser.readline().decode().strip()
  
  def write(self, data: str) -> None:
    self._ser.write(data.encode())

  def write_bytes(self, data: int) -> None:
    self._ser.write(bytes([data]))
class SerialTesting(SerialWrapper):
  def __init__(self):
    self._time = 0

  def read(self) -> str:
    import random
    voltage = random.randint(0, 5000)   # 0.000 – 5.000 V (x1000)
    dt = random.randint(50, 200)         # microseconds
    return f"{voltage}{dt}"

  def write(self, data: str):
    pass

  def write_bytes(self, data: int) -> None:
    pass

# class SerialTesting(SerialWrapper):
#   def __init__(self):
#     super().__init__()
#
#   def read(self):
#     ser = PySerial({
#       "port": "COM4",
#       "baudrate": 9600,
#       "timeout": 1
#     })
#     return ser.read()
#
#   def write(self, data):
#     pass
#
#   def write_bytes(self, data: int) -> None:
#     ser = PySerial({
#       "port": "COM4",
#       "baudrate": 9600,
#       "timeout": 1.0
#     })
#     ser.write_bytes(data)