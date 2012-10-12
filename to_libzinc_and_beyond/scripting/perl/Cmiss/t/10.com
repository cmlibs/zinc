use Cmiss::Value::Matrix;
$fred=new Cmiss::Value::Matrix(values=>[1,2,3,4,5,6],n_columns=>2);
print "$fred\n";
