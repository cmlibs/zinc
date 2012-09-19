from Cmiss.Value.Value import Value;
import Cmiss.Value.C.Matrix;
import Cmiss.Value

class Matrix(Value):

    def __init__(self, *args):
        if len(args) == 1 and type(args[0]).__module__ == 'Cmiss.Value.C' \
        and type(args[0]).__name__ == 'Value':
            Value.__init__(self, args[0]);
        else:
            value = apply(Cmiss.Value.C.Matrix.new, args);
            Value.__init__(self, value);

    def sub_matrix(self, *args, **keywords):
        return apply(Cmiss.Value.C.Matrix.sub_matrix, (self._value, ) + args, keywords)

    def get_matrix_cpointer(self):
        return Cmiss.Value.C.Matrix.get_matrix_cpointer(self._value)
