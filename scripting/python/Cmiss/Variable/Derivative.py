from Cmiss.Variable.Variable import Variable;
import Cmiss.Variable.C.Derivative;

class Derivative(Variable):
    def __init__(self, *args):
        variable = apply(Cmiss.Variable.C.Derivative.new, args);
        Variable.__init__(self, variable);
