use Cmiss::Variable_new;
use Cmiss::Variable_new::Scalar;
$fred=new Cmiss::Variable_new::Scalar(name=>'bob',value=>5);
print "$fred\n";
