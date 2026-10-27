function message = ST2_ErrorText(error_code, library_name) % Formatea el codigo devuelto por la API con su descripcion oficial.
if nargin < 2 % Selecciona la biblioteca de la arquitectura actual cuando no se indica.
    library_name = 'api_pcie_x86'; % Conserva la compatibilidad con MATLAB de 32 bits.
    if strcmp(computer, 'PCWIN64') % Detecta MATLAB de 64 bits.
        library_name = 'api_pcie_x64'; % Utiliza la misma biblioteca que OpenSys.
    end % Finaliza la seleccion de arquitectura.
end % Finaliza la seleccion de biblioteca.
message = sprintf('%d: codigo de error no documentado', error_code); % Mantiene el codigo aunque no exista traduccion.
if ~libisloaded(library_name) % Evita llamar a una DLL que no esta cargada.
    message = sprintf('%d: descripcion no disponible; biblioteca no cargada', error_code); % Explica por que no se puede consultar el catalogo.
    return; % Conserva el error original sin provocar un segundo fallo.
end % Finaliza la comprobacion de disponibilidad.
try % La presentacion del error no debe ocultar el fallo original.
    error_buffer = repmat(char(0), 1, 4096); % Reserva espacio para las descripciones del catalogo de la DLL.
    [found, description] = calllib(library_name, 'ST2_GetError', int32(error_code), error_buffer); % Consulta el codigo concreto, no un ultimo error potencialmente antiguo.
    description = char(description); % Normaliza el texto recibido desde la biblioteca.
    null_index = find(description == char(0), 1); % Localiza el terminador de la cadena C.
    if ~isempty(null_index) % Recorta un buffer que conserve el terminador.
        description = description(1:null_index-1); % Descarta el relleno posterior al mensaje.
    end % Finaliza la eliminacion del relleno.
    description = strtrim(description); % Elimina espacios del buffer de salida.
    if found == 1 && ~isempty(description) % Solo utiliza textos asociados a un codigo reconocido.
        description = regexprep(description, '^\(Error Code -?\d+\)\s*', ''); % Evita repetir el numero incluido por el traductor C++.
        if numel(description) >= 2 && description(1) == char(39) && description(end) == char(39) % Detecta las comillas del formato del catalogo.
            description = description(2:end-1); % Conserva unicamente la descripcion interior.
        end % Finaliza la normalizacion del formato.
        message = sprintf('%d: %s', error_code, description); % Presenta siempre primero el codigo y despues su significado.
    end % Mantiene el mensaje explicito de desconocido cuando no hay traduccion.
catch % Protege la notificacion si la DLL instalada no permite consultar el error.
    message = sprintf('%d: descripcion no disponible; fallo al consultar ST2_GetError', error_code); % No reemplaza el codigo original por el de la consulta.
end % Finaliza la consulta protegida.
end % Finaliza el formateador compartido por la toolbox.
