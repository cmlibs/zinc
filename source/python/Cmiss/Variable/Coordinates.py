from Cmiss.Variable.Variable import Variable;
import Cmiss.Variable.C.Coordinates;

class Coordinates(Variable):
    def __init__(self, *args):
        variable = apply(Cmiss.Variable.C.Coordinates.new, args);
        Variable.__init__(self, variable);
