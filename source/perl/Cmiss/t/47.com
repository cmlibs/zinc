if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Integral

use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Finite_element;
use Cmiss::Function::Integral;
use Cmiss::Function::Matrix;
use Cmiss::Function_variable;
use Cmiss::Function_variable::Composite;

# set up regions
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
# check creating integral function
$fun_1a=new Cmiss::Function::Finite_element(region=>$heart,name=>'coordinates');
$var_1a=$fun_1a->element_xi();
$fun_1=new Cmiss::Function::Integral(integrand_output=>$fun_1a->output(),integrand_input=>$var_1a,region=>$heart,quadrature_scheme=>"c.Hermite*c.Hermite*l.Lagrange");
print "$fun_1\n";
print "\n";

# check evaluating with no arguments
$var_1=$fun_1->output();
$fun_2=$var_1->evaluate();
print "$var_1\n";
print "$fun_2\n";
$fun_3=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$fun_4=($fun_1a->output())->evaluate(input=>$fun_1a->element_xi(),value=>$fun_3);
print "$fun_4\n";
$fun_3=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>22),xi=>[0.5,0.5,0.5]);
$fun_4=($fun_1a->output())->evaluate(input=>$fun_1a->element_xi(),value=>$fun_3);
print "$fun_4\n";
print "\n";
