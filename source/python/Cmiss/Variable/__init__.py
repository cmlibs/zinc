
import Cmiss.Variable

from Variable import Variable

def Composite(*args):
    from Composite import Composite
    Cmiss.Variable.Composite = Composite;
    return apply(Composite, args)

def Composition(*args):
    from Composition import Composition
    Cmiss.Variable.Composition = Composition;
    return apply(Composition, args)

def Coordinates(*args):
    from Coordinates import Coordinates
    Cmiss.Variable.Coordinates = Coordinates;
    return apply(Coordinates, args)

def Derivative(*args):
    from Derivative import Derivative
    Cmiss.Variable.Derivative = Derivative;
    return apply(Derivative, args)

def Element_xi(*args):
    from Element_xi import Element_xi
    Cmiss.Variable.Element_xi = Element_xi;
    return apply(Element_xi, args)

def Finite_element(*args):
    from Finite_element import Finite_element
    Cmiss.Variable.Finite_element = Finite_element;
    return apply(Finite_element, args)

def Identity(*args):
    from Identity import Identity
    Cmiss.Variable.Identity = Identity;
    return apply(Identity, args)

def Nodal_value(*args):
    from Nodal_value import Nodal_value
    Cmiss.Variable.Nodal_value = Nodal_value;
    return apply(Nodal_value, args)

def Prolate_spheroidal_to_rectangular_cartesian(*args):
    from Prolate_spheroidal_to_rectangular_cartesian import Prolate_spheroidal_to_rectangular_cartesian
    Cmiss.Variable.Prolate_spheroidal_to_rectangular_cartesian = Prolate_spheroidal_to_rectangular_cartesian;
    return apply(Prolate_spheroidal_to_rectangular_cartesian, args)

def Spheroidal_coordinates_focus(*args):
    from Spheroidal_coordinates_focus import Spheroidal_coordinates_focus
    Cmiss.Variable.Spheroidal_coordinates_focus = Spheroidal_coordinates_focus;
    return apply(Spheroidal_coordinates_focus, args)

    
