from Cmiss.Value.Value import Value;
import Cmiss.Value.C.FE_value_vector;

class FE_value_vector(Value):

    def __init__(self, *args):
        if len(args) == 1 and type(args[0]).__module__ == 'Cmiss.Value.C' \
        and type(args[0]).__name__ == 'Value':
            Value.__init__(self, args[0]);
        else:
            value = apply(Cmiss.Value.C.FE_value_vector.new, args);
            Value.__init__(self, value);

