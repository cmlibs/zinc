#ifndef PY_CMISS_VARIABLE_H
#define PY_CMISS_VARIABLE_H
#ifdef __cplusplus
extern "C" {
#endif

extern DL_IMPORT(PyTypeObject) CmissVariableType;

#define CmissVariable_Check(op) PyObject_TypeCheck(op, &CmissVariableType)
#define CmissVariable_CheckExact(op) ((op)->ob_type == &CmissVariableType)

#ifdef __cplusplus
}
#endif
#endif /* !PY_CMISS_VARIABLE_H */
