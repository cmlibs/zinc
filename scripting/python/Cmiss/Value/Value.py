import Cmiss.Value.C.Value;

class Value:

    value = None;

    def __str__(self):
        return self._value.get_string();

    def get_value_cpointer(self):
        return self._value.get_value_cpointer();

    def __init__(self, value):
        self._value = value;
