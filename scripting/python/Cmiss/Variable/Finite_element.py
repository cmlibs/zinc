from Cmiss.Variable.Variable import Variable;
import Cmiss.Variable.C.Finite_element;

class Finite_element(Variable):
    def __init__(self, *args):
        variable = apply(Cmiss.Variable.C.Finite_element.new, args);
        Variable.__init__(self, variable);
