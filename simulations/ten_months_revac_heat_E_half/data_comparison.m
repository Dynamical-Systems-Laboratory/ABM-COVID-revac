%% Post-processing 
clear
close all

% Data directory and the mat file
mfile = 'test.mat';

% Plot settings
clrf = [0.7, 0.7, 0.7];
clrm = [21/255, 23/255, 150/255];

% Plot all realizations and the mean
load(mfile)

% Data collection starts immediately
cst = 1;

% Active
ylab = 'Active cases';
plot_title = '$\mathrm{E+S_y}$';
temp = cur_infected(:,cst:end);
plot_all_and_mean(time(cst:end), temp, 1, clrm, clrf, plot_title, ylab, false)

% Total
ylab = 'Total number of cases';
plot_title = '$\mathrm{E+S_y}$';
temp = tot_infected(:,cst:end);
plot_all_and_mean(time(cst:end), temp, 2, clrm, clrf, plot_title, ylab, false)

% Total deaths
ylab = 'Total number of deaths';
plot_title = '$\mathrm{R_D}$';
temp = tot_deaths(:,cst:end);
plot_all_and_mean(time(cst:end), temp, 3, clrm, clrf, plot_title, ylab, false)

function plot_all_and_mean(time, y, i, clrm, clrf, plot_title, ylab, noMarkers)

    % Create figure
    figure1 = figure(i);

    % Create axes
    axes1 = axes('Parent',figure1);

    for i=1:size(y,1)
        plot(time, y(i,:), 'LineWidth', 2, 'Color', clrf)
        hold on
    end
    plot(time, mean(y,1), 'LineWidth', 2, 'Color', clrm)

    % Create ylabel
    ylabel(ylab,'Interpreter','latex');

    % Create xlabel
    xlabel('Time (days)','Interpreter','latex');

    % Create title
%     title(plot_title,'Interpreter','latex');

    % Uncomment the following line to preserve the Y-limits of the axes
    % ylim(axes1,[0 5]);
    box(axes1,'on');
    % Set the remaining axes properties
    set(axes1,'FontSize',24,'TickLabelInterpreter','latex','XGrid','on','YGrid',...
        'on'); 
    
end

function plot_one(time, y, i, clrm, clrf, plot_title, ylab, noMarkers)

    % Create figure
    figure1 = figure(i);

    % Create axes
    axes1 = axes('Parent',figure1);

    for i=1:size(y,1)
        plot(time, y(i,:), 'LineWidth', 2, 'Color', clrf)
        hold on
    end
    plot(time, mean(y,1), 'v-', 'LineWidth', 2, 'Color', clrm)
 
    % Create ylabel
    ylabel(ylab,'Interpreter','latex');

    % Create xlabel
    xlabel('Time (days)','Interpreter','latex');

    % Create title
    title(plot_title,'Interpreter','latex');

    % Uncomment the following line to preserve the Y-limits of the axes
    % ylim(axes1,[0 5]);
    box(axes1,'on');
    % Set the remaining axes properties
    set(axes1,'FontSize',24,'TickLabelInterpreter','latex','XGrid','on','YGrid',...
        'on'); 
    
    % Ticks
    xticks([1 50 100 140])
    xticklabels({'March 3','April 22','June 11','July 21'})
    
    % Add events
    plot([10,10],ylim, '--', 'LineWidth', 2, 'Color', [188/255, 19/255, 30/255])
    plot([19,19],ylim, '-.', 'LineWidth', 2, 'Color', [188/255, 19/255, 30/255])
    plot([84,84],ylim, '--', 'LineWidth', 2, 'Color', [123/255, 33/255, 157/255])
    plot([98, 98],ylim, '-.', 'LineWidth', 2, 'Color', [123/255, 33/255, 157/255])
    plot([112, 112],ylim, ':', 'LineWidth', 2, 'Color', [123/255, 33/255, 157/255])    
    
end