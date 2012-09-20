function p = Cmiss_Variable(a)
%Cmiss_variable constructor.  Creates a matlab Cmiss_variable object
%   
if nargin == 0
   p.variable_pointer = 0;
   p = class(p,'Cmiss_Variable');
elseif isa(a,'Cmiss_Variable')
   p = a;
else
   p.variable_pointer = create(a);
   p = class(p,'Cmiss_Variable');
end