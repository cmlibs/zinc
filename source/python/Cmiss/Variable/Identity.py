from Cmiss.Variable.Variable import Variable;
import Cmiss.Variable.C.Identity;
import Cmiss.Variable

class Identity(Variable):
    def __init__(self, *args):
        variable = apply(Cmiss.Variable.C.Identity.new, args);
        Variable.__init__(self, variable);

