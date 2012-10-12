function p = Cmiss_Value_Matrix(a)
%Cmiss_value constructor.  Creates a matlab Cmiss_Value_Matrix object
%   
if nargin == 0
	% Make a structure with no fields
	p = struct;
	value = Cmiss_Value;
   p = class(p,'Cmiss_Value_Matrix',value);
elseif isa(a,'Cmiss_Value_Matrix')
   p = varargin;
elseif nargin == 1
   % Make a structure with no fields
	p = struct;
	tmp = create(a);
	value = Cmiss_Value(tmp);
   p = class(p,'Cmiss_Value_Matrix',value);
else
   error('Must supply exactly 0 or 1 arguments');
end
