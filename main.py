from fetcher_config.definition import *
from fetcher_config.serial_wrapper import *
from fetcher_config.port_config import *

import matplotlib.pyplot as plt
import numpy as np
from time import perf_counter, sleep
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Slider




DEBUG_MODE = 1
running = True


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

def send_delay_to_arduino(delay_us: int):
  if DEBUG_MODE:
    return

  fetcher._ser.write_bytes(DELAY_CONFIG)
  fetcher._ser.write(str(delay_us))
  fetcher._ser.write_bytes(END_CONFIG_CHANGE)


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

# def test():
#   data = np.array(fetcher.fetch_data())
#
#   print(data)
#
#   for i in range(1, len(data[:,1])):
#     data[i,1] += data[i-1,1]
#
#   fig, ax = plt.subplots()             # Create a figure containing a single Axes.
#   # ax.plot(np.arange(len(data[:,1])), data[:, 0])  # Plot some data on the Axes.
#   ax.plot(data[:,1], data[:, 0])  # Plot some data on the Axes.
#   plt.show()
#
#   pass
def on_key(event):
  global running
  if event.key == " ":
    running = not running

buffer_size = 500
time_buffer = []
voltage_buffer = []



fig, ax = plt.subplots()
plt.subplots_adjust(bottom=0.25)
ax_buf = plt.axes([0.2, 0.1, 0.6, 0.03])
buf_slider = Slider(
  ax=ax_buf,
  label="Buffer Size",
  valmin=100,
  valmax=2000,
  valinit=buffer_size,
  valstep=100
)

ax_delay = plt.axes([0.2, 0.05, 0.6, 0.03])
delay_slider = Slider(
  ax=ax_delay,
  label="Delay (us)",
  valmin=10,
  valmax=500,
  valinit=50,
  valstep=10
)

def on_delay_change(val):
  delay = int(val)
  send_delay_to_arduino(delay)

delay_slider.on_changed(on_delay_change)


def on_buffer_change(val):
  global buffer_size
  buffer_size = int(val)

buf_slider.on_changed(on_buffer_change)


fig.canvas.mpl_connect("key_press_event", on_key)

line, = ax.plot([], [], lw=2)

ax.set_xlabel("Time (us)")
ax.set_ylabel("Voltage (V)")
ax.set_title("Live Oscilloscope")
ax.grid(True)


def init():
  ax.set_xlim(0, 10000)
  ax.set_ylim(0, 5)
  return line,

def update(frame):
  if not running:
    return line,

  data = fetcher.fetch_data()

  for v, t in data:
    if len(time_buffer) == 0:
      time_buffer.append(t)
    else:
      time_buffer.append(time_buffer[-1] + t)
    voltage_buffer.append(v)

  if len(time_buffer) > buffer_size:
    time_buffer[:] = time_buffer[-buffer_size:]
    voltage_buffer[:] = voltage_buffer[-buffer_size:]

  line.set_data(time_buffer, voltage_buffer)

  ax.set_xlim(time_buffer[0], time_buffer[-1])
  ax.set_ylim(
      min(voltage_buffer) - 0.1,
      max(voltage_buffer) + 0.1
  )

  return line,


if __name__ == "__main__":
  ani = FuncAnimation(
    fig,
    update,
    init_func=init,
    interval=100,
    blit=True
  )
  plt.show()


# if __name__ == "__main__":
#   test()
#   # test_change_config()
#   # for _ in range(2):
#   #   test()
#   # fetcher._ser.write_bytes(DELAY_CONFIG)
#   # fetcher._ser.write("20")
#   # fetcher._ser.write_bytes(END_CONFIG_CHANGE)
#   # for _ in range(10):
#   #   test()