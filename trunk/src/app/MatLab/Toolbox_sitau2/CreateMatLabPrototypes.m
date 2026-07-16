%Para fabricar los thunk y el archivo prototipo utilizable con loadlibrary
%ATENCION LLEVA MAL QUE EN .h SE INDENTEN LAS DIRECTIVAS 
%(NO QUIERE LINEAS DE ORDENES AL PREPROCESADOR QUE EMPIECEN CON BLANCO)

current_path = pwd;
filepath = fileparts(mfilename('fullpath'))
cd(filepath);
loadlibrary ('stlib2_x64', 'stlib2.h', 'mfilename', 'stlib2');
unloadlibrary ('stlib2_x64');
loadlibrary ('api_pcie_x64', 'api_pcie.h', 'mfilename', 'api_pcie');
unloadlibrary ('api_pcie_x64');
cd(current_path);