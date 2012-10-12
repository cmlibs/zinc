import Cmiss.Value
from Value import Value

def Element_xi(*args):
    from Element_xi import Element_xi
    Cmiss.Value.Element_xi = Element_xi;
    return apply(Element_xi, args)

def FE_value_vector(*args):
    from FE_value_vector import FE_value_vector
    Cmiss.Value.FE_value_vector = FE_value_vector;
    return apply(FE_value_vector, args)

def Derivative_matrix(*args):
    from Derivative_matrix import Derivative_matrix
    Cmiss.Value.Derivative_matrix = Derivative_matrix;
    return apply(Derivative_matrix, args)

def Matrix(*args):
    from Matrix import Matrix
    Cmiss.Value.Matrix = Matrix;
    return apply(Matrix, args)



    
    
