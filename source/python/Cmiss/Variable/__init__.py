

from Variable import Variable

def Derivative(*args):
    import Cmiss.Variable
    from Derivative import Derivative
    Cmiss.Variable.Derivative = Derivative;
    return apply(Derivative, args)

def Element_xi(*args):
    import Cmiss.Variable
    from Element_xi import Element_xi
    Cmiss.Variable.Element_xi = Element_xi;
    return apply(Element_xi, args)

def Finite_element(*args):
    import Cmiss.Variable
    from Finite_element import Finite_element
    Cmiss.Variable.Finite_element = Finite_element;
    return apply(Finite_element, args)

def Nodal_value(*args):
    import Cmiss.Variable
    from Nodal_value import Nodal_value
    Cmiss.Variable.Nodal_value = Nodal_value;
    return apply(Nodal_value, args)

    
