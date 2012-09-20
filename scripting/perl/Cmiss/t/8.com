if (!defined $path)
{
  $path = ".";
}
use Cmiss::Value::Element_xi;
use Cmiss::Value::FE_value_vector;
use Cmiss::Value::Derivative_matrix;
use Cmiss::Variable::Derivative;
use Cmiss::Variable::Element_xi;
use Cmiss::Variable::Finite_element;
use Cmiss::Region;
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
$coordinates=new Cmiss::Variable::Finite_element(region=>$heart,name=>'coordinates');
$element_xi=new Cmiss::Value::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$element_xi_var=new Cmiss::Variable::Element_xi(name=>'element_xi_var');
$d_coordinates=new Cmiss::Variable::Derivative(name=>"d_coordinates",dependent=>$coordinates,independent=>[$element_xi_var,$element_xi_var]);
$result=$d_coordinates->evaluate($element_xi_var,$element_xi);
$matrix=$result->matrix(independent=>[$element_xi_var]);
print "$matrix\n";
Cmiss::Value::Matrix->set_string_convert_max_columns(20);
print "$result\n";
$element_xi=0;
$coordinates=0;
$heart=0;
$root=0;
