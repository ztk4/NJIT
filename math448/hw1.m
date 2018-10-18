%% Zachary Kaplan
 % Math 448
 % 9/20/18
 % HW 1
 
%% Common Data

% Global declarations for ease of reference.
% Allows helper functions to modify global state without a lot of parameter
% passing.
global A S n D i t rc rs;

A = [12 31 63 95 99 154 198 221 304 346 411 455 537];  % Arrival Times.
S = [40 32 55 48 18 50 47 18 28 54 40 72 12];          % Service Times.
n = length(A);    % Number of Customers.
D = zeros(1, n);  % Pre-allocate departure times.

%% Algorithm 1: Find the departure time for each customer with a single server.

t = 0;  % Current time.
for i = 1:n
    if t < A(i)
        % If the next customer has not arrived, advance time.
        t = A(i);
    end
    
    % Serve the next customer and advance time.
    t = t + S(i);
    % Mark departure time.
    D(i) = t;
end

% Print out the departure times.
disp("Algorithm 1: D = ")
disp(D)

%% Algorithm 2: Find the departure time for each customer with two servers

i = 1;  % Next customer.
t = 0;  % Current time.
rc = [-1 -1];  % Indices of customers being helped.
rs = [0 0];    % Remaining service time of customers heing helped.
% Helper function for sending the next customer to server j:
% See definition of AcceptNextCustomer(j) in section "Helper Methods".

% While there are more customers or any server is busy, continue.
while i <= n || any(rc ~= -1)
    if all(rc == -1)  % All servers are free.
        % Take the next customer, advancing time as needed.
        AcceptNextCustomer(1);
    else
        % Check if one of the servers are free.
        j = find(rc == -1, 1);
        if isempty(j)  % No servers are free.
            % Find the lesser of the service times remaining, and which
            % server min_idx has that time.
            [min_rs, min_idx] = min(rs);
            other_idx = mod(min_idx, 2) + 1;  % Index of other server.
            
            t = t + min_rs;  % Advance the remaining service time.
            % Deduct this time from the other server's time.
            rs(other_idx) = rs(other_idx) - min_rs;
            % Mark customer that we just finished serving as done.
            D(rc(min_idx)) = t;
            rc(min_idx) = -1;  % Mark server as open.
        else  % The j'th server is free.
            other_idx = mod(j, 2) + 1;  % Index of other (not free) server.
            
            if i <= n && A(i) < t
                % If the next customer is alredy here,
                AcceptNextCustomer(j)  % Accept them.
            elseif i <= n && A(i) - t < rs(other_idx)
                % If the next customer will arrive before the other server
                % is finished, deduct the wait time.
                rs(other_idx) = rs(other_idx) - (A(i) - t);
                % And accept the next customer.
                AcceptNextCustomer(j)
            else
                % If the other server will finish before the next customer,
                % then finish serving.
                t = t + rs(other_idx);  % Advance time.
                D(rc(other_idx)) = t;   % Mark customer as done.
                rc(other_idx) = -1;     % Mark server as open.
            end
        end
    end
end

% Print out the departure times.
disp("Algorithm 2: D = ")
disp(D)

%% Algorithm 3: Find the departure time for each customer with a single server, using stack priority.

t = 0;                % Current time
D(1:length(D)) = -1;  % Initialize D to all -1's.
while true
    % Find the most recently arrived customer who has not been helped.
    i = find(A <= t & D == -1, 1, 'last');
    if isempty(i)
        % No such customer exists, instead serve next customer to arrive.
        i = find(A > t, 1);
        if isempty(i); break; end  % No next customer, we are done.
        
        t = A(i);  % Advance time until they arrive.
    end
    
    t = t + S(i);  % Serve the current customer.
    D(i) = t;      % Mark customer as departed.
end

% Print out the departure times.
disp("Algorithm 3: D = ")
disp(D)

%% Helper Methods: Matlab requires local helper functions be defined after the rest of the script.
function AcceptNextCustomer(j)
    % We are using the global i, t, rc, rs, S, and A in this method.
    global i t rc rs S A;
    if t < A(i)
        t = A(i);  % Advance time if needed.
    end
    
    rs(j) = S(i);  % Mark down the service time of next customer.
    rc(j) = i;     % Mark down the index of next customer.
    i = i + 1;     % Increment next customer index.
end