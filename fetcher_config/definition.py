from typing import Any

type Config = dict[str, Any]
type Data = list[float, int]
type DataList = list[Data]

DELAY_TYPE_SECOND = 0
DELAY_TYPE_MILLI = 1
DELAY_TYPE_MICRO = 2

PRESCALER_CONFIG = 0
DELAY_CONFIG = 1
DELAY_TYPE_CONFIG = 2
DECIMAL_POINTS_CONFIG = 3
END_CONFIG_CHANGE = 127