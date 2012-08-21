from Cmiss.Variable.Variable import Variable;
import Cmiss.Variable.C.Composite;
import Cmiss.Variable

class Composite(Variable):
    def __init__(self, *args):
        variable = apply(Cmiss.Variable.C.Composite.new, args);
        Variable.__init__(self, variable);

