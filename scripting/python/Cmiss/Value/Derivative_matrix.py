from Cmiss.Value.Value import Value;
import Cmiss.Value.C.Derivative_matrix;

class Derivative_matrix(Value):

    def __init__(self, *args):
        if len(args) == 1 and type(args[0]).__module__ == 'Cmiss.Value.C' \
        and type(args[0]).__name__ == 'Value':
            Value.__init__(self, args[0]);
        else:
            value = apply(Cmiss.Value.C.Derivative_matrix.new, args);
            Value.__init__(self, value);

    def matrix(self, *args):
        return apply(Cmiss.Value.C.Derivative_matrix.matrix, (self._value,) + args)
