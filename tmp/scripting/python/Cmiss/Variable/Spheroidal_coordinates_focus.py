from Cmiss.Variable.Variable import Variable;
import Cmiss.Variable.C.Spheroidal_coordinates_focus;

class Spheroidal_coordinates_focus(Variable):
    def __init__(self, *args):
        variable = apply(Cmiss.Variable.C.Spheroidal_coordinates_focus.new, args);
        Variable.__init__(self, variable);
