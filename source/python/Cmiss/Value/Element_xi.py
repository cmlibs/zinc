from Cmiss.Value.Value import Value;
import Cmiss.Value.C.Element_xi;

class Element_xi(Value):

    def __init__(self, *args):
        value = apply(Cmiss.Value.C.Element_xi.new, args);
        Value.__init__(self, value);

