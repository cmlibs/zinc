from Cmiss.Variable.Variable import Variable;
import Cmiss.Variable.C.Nodal_value;

class Nodal_value(Variable):
    def __init__(self, *args):
        variable = apply(Cmiss.Variable.C.Nodal_value.new, args);
        Variable.__init__(self, variable);
