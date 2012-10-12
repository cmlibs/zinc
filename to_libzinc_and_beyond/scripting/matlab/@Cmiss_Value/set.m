function s = set(s,varargin)
% SET Set stock properties to the specified values
% and return the updated object
property_argin = varargin;
while length(property_argin) >= 2,
    prop = property_argin{1};
    val = property_argin{2};
    property_argin = property_argin(3:end);
    switch prop
    case 'Value_pointer'
        s.value_pointer = val;
    otherwise
        error('Invalid property')
    end
end

