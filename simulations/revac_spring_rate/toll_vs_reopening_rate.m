%
% Total cases and total deaths vs. testing efficacy
% Loads and plots the final numbers vs. the testing efficacy
%

clear; close all

% Input
% Number of independent simulations in each set
num_sim = 100;
% Common file name
mname = 'dir';

% Testing efficacies (x-axis)
testing_Sy = [0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0];

% Files to consider
dir_names = dir([mname, '_*']);
dir_names = {dir_names.name};
ndirs = length(dir_names);
% Extract the numbers in file endings 
str=sprintf('%s#', dir_names{:});
num = sscanf(str, [mname,'_%f#']);
% Sort by the numbers (here just index in the input array)
[~,idx]=sort(num);

total_cases = zeros(num_sim, ndirs);
total_deaths = zeros(num_sim, ndirs);

for ii = 1:length(idx)
    % To see the current file
    fprintf('Processing: %s\n', dir_names{idx(ii)})
    temp = load([dir_names{idx(ii)},'/sim_results.mat']);
    total_deaths(:,ii) = temp.tot_deaths(:,end);
    total_cases(:,ii) = temp.tot_infected(:,end);
end

%
% Plot results from all simulations
%

% Plot settings
clrf = [104,180,76]/255; 
clrm = [0,85,0]/255;

plot_title = '';
ylab = 'Total deaths';
ylimits = [0,200];
plot_all_and_mean(testing_Sy, total_deaths, 1, clrm, clrf, plot_title, ylab, ylimits)

% ylab = 'Total infected';
% ylimits = [0,80000];
% plot_all_and_mean(testing_Sy, total_cases, 2, clrm, clrf, plot_title, ylab, ylimits)


function plot_all_and_mean(x, y, i, clrm, clrf, plot_title, ylab, ylimits)

    % Create figure
    figure1 = figure(i);

   
    % Create axes
    axes1 = axes('Parent',figure1);
 
    % Convert to %
     x = x'*100;
    
% To plot each simulation individually + the mean
%     for i=1:size(y,1)
%         plot(x, y(i,:), 'o', 'LineWidth', 2, 'MarkerSize', 8,  'Color', clrf)
%         hold on
%     end
%     ym = mean(y,1);
%     
%     curve1 = min(y);
%     curve2 = max(y);
%     x2 = [x, fliplr(x)];
%     inBetween = [curve1, fliplr(curve2)];
% 
%     fill(x2, inBetween, clrf, 'FaceColor', clrf, 'EdgeColor',clrf);
%     hold on;
%     plot(x, ym, 'o-', 'LineWidth', 2, 'MarkerSize', 8, 'Color', clrm, 'MarkerFaceColor', clrm);

    % Whisker plot
%     boxplot(y, x,'BoxStyle', 'filled', 'Colors', clrf, 'Symbol', '+', 'positions', x)

    boxplot(y, x,'BoxStyle', 'filled', 'Colors', clrf, 'Symbol', '+')
    % Box width
    a = get(get(gca,'children'),'children');   % Get the handles of all the objects
    t = get(a,'tag');   % List the names of all the objects 
    idx=strcmpi(t,'box');  % Find Box objects
    boxes=a(idx);          % Get the children you need
    set(boxes,'linewidth',10); % Set width
    % Median color
    lines = findobj(gcf, 'type', 'line', 'Tag', 'Median');
    set(lines, 'Color', clrm, 'LineWidth', 2);
    % Outliers
    ho = findobj(gcf,'tag','Outliers');
%     for j = 1:numel(n)
%     if rem(n(j).XData(1),2)~=0
%        n(j).MarkerEdgeColor = rgb('DeepSkyBlue');
%     end
    set(ho, 'MarkerEdgeColor', clrm, 'LineWidth', 2)
    
    % Ticks and tick labesl
%     ax = gca;
%     ax.Xticklabels = 
    
%     % Create ylabel
% %     ylabel(ylab,'Interpreter','latex');
    ylabel(ylab);

    % Create xlabel
%     xlabel('Reopening rate, \%/day','Interpreter','latex');
    xlabel('Testing efficacy, %Sy tested');

    % Create title
    title(plot_title,'Interpreter','latex');

     % Uncomment the following line to preserve the Y-limits of the axes
    % ylim(axes1,[0 5]);
    box(axes1,'on');
    % Set the remaining axes properties
%     set(axes1,'FontSize',24,'TickLabelInterpreter','latex','XGrid','on','YGrid',...
%         'on'); 
     set(axes1,'FontSize',28,'FontName','SanSerif','XGrid','off','YGrid',...
        'off'); 
  
%     set(axes1, 'XScale', 'log');
    ax = gca;
%     ax.YAxis.Exponent = 0;
%     ax.YAxis.TickLabelFormat = '%,.0f';
% ax.YAxis.TickLabelFormat = '%.f';
    
%     xlim([0 10])
%     xticks([0 2.5 5 7.5 10])
    ylim(ylimits)
    
    set(gcf,'Position',[200 500 950 750])
end