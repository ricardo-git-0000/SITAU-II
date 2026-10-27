function [error] = OpenSys (path, ip, tcp_port_cfg, tcp_port_acq, pcie_enabled, emulates_sitau2)


% (load_matlab_prototype_file = true) -> Loads library using MatLab prototype file (.m)
% (load_matlab_prototype_file = false) -> Loads library using c++ prototype file (.h)
load_matlab_prototype_file = false;     

if nargin < 6
    emulates_sitau2 = false;
end

if emulates_sitau2 == false
    gb_sitau_no_hardware = 0;
else
    gb_sitau_no_hardware = 1;
end
gb_sitau_load_modules = 2;
gb_sitau_load_uci = 2;

nlib = 'api_pcie';
%nlib_h = ['C:\sitau2_openfus\Sitau2\V03\Toolbox_sitau2\' nlib '.h'];
nlib_h = [nlib '.h'];
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    %nlib = ['C:\sitau2_openfus\Sitau2\V03\Toolbox_sitau2\' nlib '_x64'];
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
nlib_dll = [nlib '.dll'];

error = libisloaded (nlib);
if error ~= 1
    if load_matlab_prototype_file == false
        [notfound, warnings] = loadlibrary(nlib_dll, nlib_h);
        if length(warnings) > 0
            warnings
        end
    else
        if exist('api_pcie.m', 'file') ~= 2
            CreateMatLabPrototypes();
        end
        [notfound, warnings] = loadlibrary(nlib_dll, @api_pcie);
        if length(warnings) > 0
            warnings
        end
    end
    error = libisloaded (nlib);
    if error ~= 1
        errordlg(['   Error: "' nlib_dll '" library was not loaded']);
        return;
    end
end

ST2_SetNoHardware(gb_sitau_no_hardware);
ST2_EnabledLoadUCIFirmware(gb_sitau_load_uci);
ST2_SITAUHardwareEnabled(gb_sitau_load_modules);

lib_version = repmat([' '], 1, 255);
[lib_version] = calllib(nlib,'PCIe_GetLibraryVersion',lib_version);
disp(['API_PCIe.DLL v' lib_version]);

[lib_version] = calllib(nlib,'ST2_InfoLibraryVersion',lib_version);
disp(['SITAU API v' lib_version]);

error = calllib(nlib,'ST2_OpenSys', path, ip, tcp_port_cfg, tcp_port_acq, pcie_enabled);
if (error < 0)
    disp(['Error ST2_OpenSys(): ' ST2_ErrorText(error, nlib)]) % Muestra el codigo junto a su descripcion de la DLL.
    calllib(nlib,'ST2_CloseSys');
    return;
end

firmware_ethernet = repmat([' '], 1, 255);
firmware_uci_sw = repmat([' '], 1, 255);
firmware_uci_hw = repmat([' '], 1, 255);
firmware_mk32_hw = repmat([' '], 1, 255);
[error firmware_ethernet firmware_uci_sw firmware_uci_hw firmware_mk32_hw] = calllib(nlib, 'ST2_InfoFirmwareVersion', firmware_ethernet, firmware_uci_sw, firmware_uci_hw, firmware_mk32_hw);
if (error < 0)
    disp(['Error ST2_InfoFirmwareVersion():' ST2_ErrorText(error, nlib)]) % Muestra el codigo junto a su descripcion de la DLL.
    calllib(nlib,'ST2_CloseSys');
    return;
end

result = calllib(nlib,'ST2_GetNoHardware');

disp(['SITAU Ethernet Firmware Version: ' firmware_ethernet]);
disp(['SITAU UCI SW Firmware Version:   ' firmware_uci_sw]);
disp(['SITAU UCI HW Firmware Version:   ' firmware_uci_hw]);
disp(['SITAU MK32 HW Firmware Version:   ' firmware_mk32_hw]);

if pcie_enabled == 1
    pcie_id = repmat([' '], 1, 255);
    pcie_olink_signature = repmat([' '], 1, 255);
    pcie_fuse_dma_signature = repmat([' '], 1, 255);
    pcie_emul_signature = repmat([' '], 1, 255);
    [pcie_id pcie_olink_signature pcie_fuse_dma_signature pcie_emul_signature] = calllib(nlib, 'PCIe_GetSystemInfo', pcie_id, pcie_olink_signature, pcie_fuse_dma_signature, pcie_emul_signature);
    if (error < 0)
        errordlg(['   ST2_InfoFirmwareVersion error:' ST2_ErrorText(error, nlib)]) % Muestra el codigo junto a su descripcion de la DLL.
    end
    disp(pcie_id);
    disp(pcie_olink_signature);
    disp(pcie_fuse_dma_signature);
    disp(pcie_emul_signature);
end

end
