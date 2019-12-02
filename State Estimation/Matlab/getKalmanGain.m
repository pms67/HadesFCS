function K = getKalmanGain(P, C, r)

    R = eye(3) * r;

    K = P * C' / (C * P * C' + R);

end