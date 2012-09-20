function p = Cmiss_Value_Derivative_matrix(a,b,c)
%Cmiss_value constructor.  Creates a matlab Cmiss_Value_Matrix object
%   
if nargin == 0
	% Make a structure with no fields
	p = struct;
	value = Cmiss_Value;
   p = class(p,'Cmiss_Value_Derivative_matrix',value);
elseif isa(a,'Cmiss_Value_Derivative_matrix')
   p = varargin;
elseif nargin == 3
   % Make a structure with no fields
	p = struct;
	tmp = create(a,b,c);
	value = Cmiss_Value(tmp);
   p = class(p,'Cmiss_Value_Derivative_matrix',value);
else
   error('Must supply exactly 0 or 3 arguments');
end
