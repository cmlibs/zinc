from Cmiss.Variable.Variable import Variable;
import Cmiss.Variable.C.Composition;

class Composition(Variable):
    def __init__(self, *args):
        variable = apply(Cmiss.Variable.C.Composition.new, args);
        Variable.__init__(self, variable);
