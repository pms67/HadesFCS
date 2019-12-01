% 1) Load UAVData
% 2) Generate sensor structure

mx=uavSensors.mag3D_unitVector_in_body(:,1);
my=uavSensors.mag3D_unitVector_in_body(:,2);
mz=uavSensors.mag3D_unitVector_in_body(:,3);

N = length(mx);

% CALCULATE HARD-IRON OFFSET
XTX  = zeros(4);
XTY  = zeros(4, 1);

for i = 1 : N
   
    XTX(1,1) = XTX(1,1) + mx(i) ^ 2.0;
    XTX(1,2) = XTX(1,2) + mx(i) * my(i);
    XTX(1,3) = XTX(1,3) + mx(i) * mz(i);
    XTX(1,4) = XTX(1,4) + mx(i);
    
    XTX(2,1) = XTX(2,1) + mx(i) * my(i);
    XTX(2,2) = XTX(2,2) + my(i) ^ 2.0;
    XTX(2,3) = XTX(2,3) + my(i) * mz(i);
    XTX(2,4) = XTX(2,4) + my(i);
    
    XTX(3,1) = XTX(3,1) + mx(i) * mz(i);
    XTX(3,2) = XTX(3,2) + my(i) * mz(i);
    XTX(3,3) = XTX(3,3) + mz(i) ^ 2.0;
    XTX(3,4) = XTX(3,4) + mz(i);
    
    XTX(4,1) = XTX(4,1) + mx(i);
    XTX(4,2) = XTX(4,2) + my(i);
    XTX(4,3) = XTX(4,3) + mz(i);
    XTX(4,4) = XTX(4,4) + 1.0;
    
    mmag = mx(i) ^ 2.0 + my(i) ^ 2.0 + mz(i) ^ 2.0;
    XTY(1,1) = XTY(1,1) + mx(i) * mmag;
    XTY(2,1) = XTY(2,1) + my(i) * mmag;
    XTY(3,1) = XTY(3,1) + mz(i) * mmag;
    XTY(4,1) = XTY(4,1) + mmag;
    
end

BETA = XTX^(-1) * XTY;

V = 0.5 * [BETA(1); BETA(2); BETA(3)];
B = sqrt(BETA(4) + V(1) ^ 2.0 + V(2) ^ 2.0 + V(3) ^ 2.0);

