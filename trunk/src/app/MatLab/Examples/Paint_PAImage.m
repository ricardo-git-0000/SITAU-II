function [error] = Paint_PAImage (data, start_mm, end_mm, x_coordinates, z_coordinates, angle, n_samples, n_fl, h_figure)

    error = 0;
    size_bscan = n_samples * n_fl;

    R = 0:1:n_samples-1;    
    R = (end_mm - start_mm) * R / (n_samples - 1) + start_mm;
    
    x = zeros(n_samples, n_fl); 
    y = x;
    for j=1:length(angle)
        x(:,j)= R * sin(angle(j)) + x_coordinates(j);
        y(:,j)= -R * cos(angle(j)) + z_coordinates(j);
    end
    
	figure(h_figure);
    
% B-Scan Data _____________________________________________________________
        
    bscan_img = reshape(data, n_samples, n_fl);
    pcolor(x,y,bscan_img);           
    xlabel('E-Scan (mm)');
    ylabel('Range (mm)');
%                 axis equal
    shading interp;
    drawnow;
end
