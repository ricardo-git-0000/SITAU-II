function [valid, withWarnings, performance] = Validate_XMLConfig(xmlFileWithPath)

% If no path is given, the current folder is assumed
if nargin < 1 || isempty(xmlFileWithPath)
        xmlFilePath = '\SITAU.xml';
end

% Variables
valid = false;
withWarnings = false;
warningMessages = [];

% Constants
ERR_FORMAT = "ERROR :: Invalid XML format :: ";
ERR_VALUES = "ERROR :: Invalid XML values :: ";
WRN_GENERAL = "WARNING :: ";
OPTIONAL = true;
PCIE_BANDWIDTH_HARD_LIMIT = 3.2; %GB/s
PCIE_BANDWIDTH_WARNING_LIMIT = 2.5; %GB/s 
GBE_BANDWIDTH_HARD_LIMIT = 0.07; %GB/s
GBE_BANDWIDTH_WARNING_LIMIT = 0.05; %GB/s 
FIXED_MODE_MEMORY_SIZE = 1; %GB

ADC_BASE_FREQ = 62.5; % Base sampling frequency
LINE_ACQ_OVH = 1;     % Line acquisition overhead (us)

% Load xml file

% Normalize file name
xmlFilePath = fullfile(xmlFileWithPath);

% Read file
doc = xmlread(xmlFilePath);

% Get relevant information
prf_frequency = getXLMparam(doc, "prf_frequency"); %Hz
sampling_freq_decimation = getXLMparam(doc, "sampling_freq_decimation");
material_velocity = getXLMparam(doc, "material_velocity", OPTIONAL) / 1000; %mm/us
acq_start_samples = getXLMparam(doc, "acq_start_samples", OPTIONAL);
acq_n_samples = getXLMparam(doc, "acq_n_samples", OPTIONAL);
acq_start_distance = getXLMparam(doc, "acq_start_distance", OPTIONAL); %mm
acq_end_distance = getXLMparam(doc, "acq_end_distance", OPTIONAL); %mm
processing_type = getXLMparam(doc, "mc_processing_type"); %0:none, 1:avg
if processing_type == 1
    average_signals_number = 2 ^ getXLMparam(doc, "mc_average_signals_number");
else
    average_signals_number = 1;
end
acquisition_mode = getXLMparam(doc, "acquisition_mode");
fp_focal_law_type = getXLMparam(doc, "fp_focal_law_type");

if acquisition_mode == 0 %Fixed
    fp_burst_number = getXLMparam(doc, "fp_burst_number");
else %Continuous
    fp_burst_number = inf;
end

fp_burst_images_number = getXLMparam(doc, "fp_burst_images_number");
fp_pwic_start_angle = getXLMparam(doc, "fp_pwic_start_angle", OPTIONAL);
fp_pwic_end_angle = getXLMparam(doc, "fp_pwic_end_angle", OPTIONAL);
fp_pwic_step_angle = getXLMparam(doc, "fp_pwic_step_angle", OPTIONAL);
fp_pwic_angles = getXLMparam(doc, "fp_pwix_angles", OPTIONAL);
probe_elements_number = getXLMparam(doc, "probe_elements_number");

data_link_mode = getXLMparam(doc, "data_link_mode");

% Check ranges and get number of samples
if (isempty(acq_start_samples) || isempty(acq_n_samples))

    % Distances has to be present
    if (isempty(material_velocity) || isempty(acq_start_distance) || isempty (acq_end_distance))        
        error(ERR_FORMAT + " Material velocity, start distance or end distance missing")
    else

        if (~isempty(acq_start_samples) || ~isempty(acq_n_samples))
            error(ERR_FORMAT + " Acquisition range has to be defined in samples or distances, not both")
        end
    end

    % Get samples
    acq_start_samples = round(2 * acq_start_distance / material_velocity * (ADC_BASE_FREQ / sampling_freq_decimation));
    acq_n_samples = round(2 * (acq_end_distance - acq_start_distance) / material_velocity * (ADC_BASE_FREQ / sampling_freq_decimation));
end

if fp_focal_law_type == 0 %FMC   

    warning(WRN_GENERAL + " XML validation of the FMC mode is not yet implemented");
    withWarnings = true;
    
    number_of_focal_laws = probe_elements_number; %<<<<<<<<<<<<<<<<<<<< JUST FOR DEBUGING

elseif fp_focal_law_type == 1 %PWIC

    %Check paremeter coherence
    if (isempty(fp_pwic_start_angle) && isempty(fp_pwic_end_angle) && isempty(fp_pwic_step_angle))
        if isempty(fp_pwic_angles)
            error(ERR_FORMAT + " PWIC angles configuration missing");
        else
            number_of_focal_laws = length(fp_pwic_angles);
        end
    else
        if (isempty(fp_pwic_start_angle) || isempty(fp_pwic_end_angle) || isempty(fp_pwic_step_angle))
            error(ERR_FORMAT + " PWIC start angle, end angle or step angle is missing");
        else
            number_of_focal_laws = round((fp_pwic_end_angle - fp_pwic_start_angle) / fp_pwic_step_angle) + 1;
        end
    end

elseif fp_focal_law_type == 2 %User
    error(WRN_GENERAL + " XML validation of the ""User file focal law"" mode is not yet implemented");
    withWarnings = true;
    
    number_of_focal_laws = probe_elements_number; %<<<<<<<<<<<<<<<<<<<< JUST FOR DEBUGING
else

    error(ERR_VALUES + " Unknown value in fp_focal_law_type");
end

% Common variables
sampling_time = 1 / (ADC_BASE_FREQ / sampling_freq_decimation); %us
prf_time = 1 / prf_frequency * 1e6; %us

% Check PRF
focal_law_acq_time = (acq_start_samples + acq_n_samples - 1) * sampling_time + LINE_ACQ_OVH; %us

if focal_law_acq_time > prf_time
    error(ERR_VALUES + " PRF too high. Please reduce PRF and/or number of samples")
end

% Calculate performance data
focal_law_acq_time = prf_time * average_signals_number; %us
image_acq_time = focal_law_acq_time * number_of_focal_laws; %us
burst_acq_time = image_acq_time * fp_burst_images_number; %us
focal_law_PRF = 1 / focal_law_acq_time * 1e6; %Hz
image_PRF = 1 / image_acq_time * 1e6; %Hz
burst_PRF = image_PRF / fp_burst_images_number; %Hz
acq_total_time = burst_acq_time * fp_burst_number; %us

% Check data bandwidth and memory size
focal_law_size_MB =  probe_elements_number * acq_n_samples * 2 / (1024^2);
image_size_MB = focal_law_size_MB * number_of_focal_laws;
burst_size_MB = image_size_MB * fp_burst_images_number;
total_data_GB = burst_size_MB * fp_burst_number / 1024;
required_bandwidth_GBPS = burst_size_MB / 1024 / (burst_acq_time / 1e6);


if data_link_mode == 0 %gbE    

    data_bandwidth_hard_limit = GBE_BANDWIDTH_HARD_LIMIT;
    data_bandwidth_soft_limit = GBE_BANDWIDTH_WARNING_LIMIT;

elseif data_link_mode == 1 %PCIe

    data_bandwidth_hard_limit = PCIE_BANDWIDTH_HARD_LIMIT;
    data_bandwidth_soft_limit = PCIE_BANDWIDTH_WARNING_LIMIT;

else
    error(ERR_VALUES + " Unknown value in data_link_mode");
end

required_bandwidth_perc = required_bandwidth_GBPS / data_bandwidth_hard_limit * 100;

if required_bandwidth_GBPS > data_bandwidth_hard_limit
    
    message = sprintf("Required bandwidth (%.1f GBPS) is larger than the link bandwidth (%.1f GBPS).\n Please reduce the PRF, the number of focal laws and/or the acquisition depth", ...
    round(required_bandwidth_GBPS), data_bandwidth_hard_limit);

    error(message);

elseif required_bandwidth_GBPS > data_bandwidth_soft_limit
    
    message = sprintf("Required bandwidth is above %d%% of the link bandwidth (%.1f GBPS).\n Acquisition will probably be paused if the data buffer is not read fast enough by the user", ...
    round(required_bandwidth_perc), data_bandwidth_hard_limit);

    warning(message);
    warningMessages = [warningMessages, WRN_GENERAL + " " + message];
    withWarnings = true;

end

% Get theoretical values for this configuration

performance.samples_per_focal_law = acq_n_samples;
performance.focal_laws_per_image = number_of_focal_laws;
performance.images_per_burst = fp_burst_images_number;
performance.number_of_bursts = fp_burst_number;
performance.averaging = average_signals_number;
performance.focal_law_acq_time_ms = focal_law_acq_time / 1e3;
performance.image_acq_time_ms = image_acq_time / 1e3;
performance.burst_acq_time_ms = burst_acq_time / 1e3;
performance.focal_law_PRF_Hz = focal_law_PRF;
performance.image_PRF_Hz = image_PRF;
performance.burst_PRF_Hz = burst_PRF;
performance.acq_total_time_sec = acq_total_time / 1e6;
performance.image_size_MB = image_size_MB;
performance.burst_size_MB = burst_size_MB;
performance.acq_total_size_GB = total_data_GB;
performance.required_bandwidth_GBPS = required_bandwidth_GBPS;
performance.required_bandwidth_PERC = required_bandwidth_perc;
performance.warning_messages = warningMessages;

% Show results

% Mostrar resultados de manera legible
fprintf('\n----------------------------------------------\n');
fprintf('Configuration Summary:                        \n');
fprintf('----------------------------------------------\n');
fprintf('Number of Averaged Signals       : %d\n', performance.averaging);
fprintf('Number of Samples per Focal Law  : %d\n', performance.samples_per_focal_law);
fprintf('Number of Focal Laws per Image   : %d\n', performance.focal_laws_per_image);
fprintf('Number of Images per Burst       : %d\n', performance.images_per_burst);
fprintf('Number of Bursts                 : %d\n', performance.number_of_bursts);
fprintf('----------------------------------------------\n');
fprintf('Acquisition Time:\n');
fprintf('----------------------------------------------\n');
fprintf('Focal Law Acquisition Time       : %.2f ms\n', performance.focal_law_acq_time_ms);
fprintf('Image Acquisition Time           : %.2f ms\n', performance.image_acq_time_ms);
fprintf('Burst Acquisition Time           : %.2f ms\n', performance.burst_acq_time_ms);
fprintf('Total Acquisition Time           : %.2f sec\n', performance.acq_total_time_sec);
fprintf('----------------------------------------------\n');
fprintf('Repetition Frequency:\n');
fprintf('----------------------------------------------\n');
fprintf('Focal Law Repetition Frequency   : %.2f Hz\n', performance.focal_law_PRF_Hz);
fprintf('Image Repetition Frequency       : %.2f Hz\n', performance.image_PRF_Hz);
fprintf('Burst Repetition Frequency       : %.2f Hz\n', performance.burst_PRF_Hz);
fprintf('----------------------------------------------\n');
fprintf('Data Size:\n');
fprintf('----------------------------------------------\n');
fprintf('Image Size                       : %.2f MB\n', performance.image_size_MB);
fprintf('Burst Size                       : %.2f MB\n', performance.burst_size_MB);
fprintf('Total Data Size                  : %.2f GB\n', performance.acq_total_size_GB);
fprintf('----------------------------------------------\n');
fprintf('Data Bandwidth:\n');
fprintf('----------------------------------------------\n');
fprintf('Required Bandwidth               : %.2f GBPS\n', performance.required_bandwidth_GBPS);
fprintf('Required Link Capacity           : %.2f%%\n', performance.required_bandwidth_PERC);
fprintf('----------------------------------------------\n');

