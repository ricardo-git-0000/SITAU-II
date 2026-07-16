function [x z] = FL_PACoordinates (n_elements, pitch, radius)

felement_dist = 0;
if radius == 0
    for i=1:n_elements
        x(i) = felement_dist + (i - 1) * pitch;
        z(i) = 0;
    end
else
    alfa_0 = asin((n_elements-1)*pitch/(2*radius));
    for i=1:n_elements
        alfa_i = alfa_0 - pitch*(i-1)/radius;
        x(i) = radius * (sin(alfa_0) - sin(alfa_i)); %width/2;
        z(i) = radius * (cos(alfa_i) - cos(alfa_0));        
    end
end
end

