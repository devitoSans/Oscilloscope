from fetcher_config.definition import *
from fetcher_config.serial_wrapper import *
from fetcher_config.port_config import *

import matplotlib.pyplot as plt
import numpy as np
from time import perf_counter, sleep

DEBUG_MODE = 0

class FetcherData:
  def __init__(self, 
               port: str | None = None, 
               baudrate: int = 9600, 
               timeout: float = 1.0, 
               ):
    self.port = (PORT if port is None else port)
    self.baudrate = baudrate
    self.decimal_points = 3
    self.max_batch_num = 100
    self.delay = 2
    self.delay_type = DELAY_TYPE_MICRO # pyright: ignore[reportUndefinedVariable]
    self.timeout = timeout

    self._ser = SerialTesting() if DEBUG_MODE else PySerial({
      "port": self.port,
      "baudrate": self.baudrate,
      "timeout": self.timeout
    })

  def get_config(self) -> Config:
    return dict(self.__dict__)
  
  def set_config(self, config: Config) -> None:
    for key, value in config.items():
      if not hasattr(self, key):
        continue
      setattr(self, key, value)

  # Return Voltage (Volts), Time (Micro seconds)
  def decode_data(self) -> Data:
    encodedData = self._ser.read()

    # Get until data is valid or timed out
    init_time = perf_counter()
    timedout = False
    while encodedData == "" or encodedData.__len__() <= 4:
      if perf_counter()-init_time > self.timeout:
        timedout = True
        break
      encodedData = self._ser.read()
    
    if timedout:
      print("Warning on FetcherData object: Cannot decode data within the given timeout "
      "(Possibly no data are coming from serial port anymore). Defaulting to 0.")
      return [0,0]

    voltage = float( encodedData[0:self.decimal_points+1] ) / (10**self.decimal_points)
    time = int(encodedData[self.decimal_points+1:]) # micro seconds
    
    return [voltage, time]

  def fetch_data(self) -> DataList:
    dataList = [ self.decode_data() for _ in range(self.max_batch_num) ]
    return dataList

  pass

fetcher = FetcherData()

def test_change_config():
  fetcher._ser.write_bytes(DELAY_CONFIG)
  fetcher._ser.write("50")
  fetcher._ser.write_bytes(END_CONFIG_CHANGE)

  # testSerial = PySerial({
  #     "port": "COM4",
  #     "baudrate": 9600,
  #     "timeout": 1
  # })

  # testSerial.write_bytes(1)
  # testSerial.write("2000000")
  # testSerial.write_bytes(127)

  # print(fetcher._ser.read())
  # print(testSerial.read())

  sleep(2)

def test():
  data = np.array(fetcher.fetch_data())

  print(data)

  for i in range(1, len(data[:,1])):
    data[i,1] += data[i-1,1]

  fig, ax = plt.subplots()             # Create a figure containing a single Axes.
  # ax.plot(np.arange(len(data[:,1])), data[:, 0])  # Plot some data on the Axes.
  ax.plot(data[:,1], data[:, 0])  # Plot some data on the Axes.
  plt.show()  

  pass

if __name__ == "__main__":
  test()
  # test_change_config()
  # for _ in range(2):
  #   test()
  # fetcher._ser.write_bytes(DELAY_CONFIG)
  # fetcher._ser.write("20")
  # fetcher._ser.write_bytes(END_CONFIG_CHANGE)
  # for _ in range(10):
  #   test()