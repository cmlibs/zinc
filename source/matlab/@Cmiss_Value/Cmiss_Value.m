function p = Cmiss_Value(a)
%Cmiss_value constructor.  Creates a matlab Cmiss_Value object
%   
if nargin == 0
   p.value_pointer = create;
   p = class(p,'Cmiss_Value');
elseif isa(a,'Cmiss_Value')
   p = a;
elseif (nargin == 1) && (size(a, 1) == 1) && (size(a, 2) == 1) && isa(a,'uint32')
%We are assuming that if we are getting a single uint32 then this contains a pointer
%from one of the Cmiss_value types and so we are going to reference it directly.
   p.value_pointer = a;
   p = class(p,'Cmiss_Value');
else
   error('No arguments for creating a Cmiss_Value.');
end