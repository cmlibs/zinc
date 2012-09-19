from Cmiss.Variable.Variable import Variable;
import Cmiss.Variable.C.Prolate_spheroidal_to_rectangular_cartesian;

class Prolate_spheroidal_to_rectangular_cartesian(Variable):
    def __init__(self, *args):
        variable = apply(Cmiss.Variable.C.Prolate_spheroidal_to_rectangular_cartesian.new, args);
        Variable.__init__(self, variable);
