import Cmiss.Variable.C.Variable;

class Variable:

    _variable = None;

    def evaluate(self, variable_value_list):
        return Cmiss.Variable.C.Variable.evaluate(self._variable, variable_value_list);
    
    def get_variable_cpointer(self):
        return self._variable.get_variable_cpointer();
    
    def __init__(self, variable):
        if type(variable).__module__ == 'Cmiss.Variable.C' \
        and type(variable).__name__ == 'Variable':
            self._variable = variable;
        else:
            self._variable = Cmiss.Variable.C.Variable.new(variable);
