from Cmiss.Variable.Variable import Variable;
import Cmiss.Variable.C.Element_xi;
import Cmiss.Variable

class Element_xi(Variable):

    def __init__(self, *args):
        variable = apply(Cmiss.Variable.C.Element_xi.new, args);
        Variable.__init__(self, variable);

#Make the class a member of the Cmiss.Variable module
Cmiss.Variable.Element_xi = Element_xi

