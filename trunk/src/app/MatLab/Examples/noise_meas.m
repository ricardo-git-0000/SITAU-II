function [noise_std,noise_peak] = noise_meas(img,filter)
%NOISE_MEAS Summary of this function goes here
%   Detailed explanation goes here
[n_samples,n_ch]= size(img);


hpf_cutoff = 7; %HPF cutoff freq
hpf_order = 48; %HPF order
fs = 62.5; %sampling frequency MHz

img=double(img);
%FILTER
if(filter)
    B = fir1(hpf_order,hpf_cutoff/(fs/2),'high');
    img=filtfilt(B,1,img);
end

noise_std=zeros(1,n_ch);
noise_peak=zeros(1,n_ch);
for i=1:n_ch
   noise_std(i)=std(img(:,i));
   noise_peak(i)=max(abs(img(:,i)));
end


end

