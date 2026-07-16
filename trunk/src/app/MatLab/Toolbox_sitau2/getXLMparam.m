function value=getParam02(xml, parname, optionalParameter)

if nargin < 3 
    optionalParameter = false;
end

ptrPar=getElementsByTagName(xml,'Parameter');
nParam=ptrPar.getLength;

ip=0;
found=false;
while ip<nParam && ~found
    par=item(ptrPar,ip);
    if strcmp(char(par.getAttribute('pName')),parname)
       found=true;
       parfound=par;
    end    
    ip=ip+1;
end

if ~found
    if ~optionalParameter
        error(sprintf('ERROR :: Invalid XML format :: Parameter %s not found', parname));
    else
        value = [];        
        return
    end
end

value=str2num(parfound.getAttribute('pValue'));

end

