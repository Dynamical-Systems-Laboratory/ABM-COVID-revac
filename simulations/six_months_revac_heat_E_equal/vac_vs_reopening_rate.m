%
% Total cases and total deaths in revacinating vs. testing efficacy 
% Loads and plots the final numbers vs. testing efficacy 
%

clear; close all

% Input
% Town population
n_pop = 79205;
% Number of independent simulations in each set
num_sim = 100;
% Common file name
mname = 'dir';

% Files to consider
dir_names = dir([mname, '_*']);
dir_names = {dir_names.name};
ndirs = length(dir_names);
% Extract the numbers in file endings 
str=sprintf('%s#%s#', dir_names{:});
num = sscanf(str, [mname,'_%f_%d#']);
% Numbers to consider
vac_num = sort(unique(num(2:2:end)));
tst_num = sort(unique(num(1:2:end)));
% For array sizes
n_vac = length(vac_num);
n_tst = length(tst_num);

% These will be average values over all realizations
total_cases = zeros(n_vac, n_tst);
total_deaths = zeros(n_vac, n_tst);

% x (%Sy tested) and y (vaccination) axes
testing_Sy = tst_num;
vac_rates = vac_num;

for ii = 1:n_vac
    for jj = 1:n_tst
        % Name of the directory, this may crash if the numbers are not in
        % the same format as the ones in the directory name
        str = sprintf('%.1f',tst_num(jj));
        str = regexprep(str,'([1-9])[0]+','$1');
        dname = sprintf([mname,'_%s_%d'], str, vac_num(ii));
        fprintf('Processing: %s\n', dname)
        temp = load([dname,'/sim_results.mat']);
        % Averages over all realizations
        total_deaths(ii,jj) = mean(temp.tot_deaths(:,end));
        total_cases(ii,jj) = mean(temp.tot_infected(:,end));
    end
end

%
% Plot results from all simulations
%

% Plot settings


% Color for the largest value in the heatmap
max_clr = [7, 125, 5]/255;
% Number of colors to use (lowest is white)
clr_points = 21;

plot_title = 'Total deaths';
ylimits = [0,500];
clevels = [5, 10, 25, 50, 100, 150, 250];
cb_ticks = [0, 100, 200, 300, 400, 500];
cb_tick_labels = {'0', '100', '200', '300', '400', '500'};
plot_heatmap(testing_Sy, vac_rates, total_deaths, 1, plot_title, max_clr, clr_points, ylimits(2), 1, n_pop, cb_ticks, cb_tick_labels, clevels)

plot_title = 'Total infected';
ylimits = [0,1.5e5];
clevels = [700, 1e3, 2e3, 3.5e3, 7e3, 2.5e4, 6e4];
cb_ticks = [0, 0.25e5, 0.5e5, 0.75e5, 1e5, 1.25e5, 1.5e5];
cb_tick_labels = {'0', '0.25', '0.5', '0.75', '1', '1.25', '1.5'};
plot_heatmap(testing_Sy, vac_rates, total_cases, 2, plot_title, max_clr, clr_points, ylimits(2), 1, n_pop, cb_ticks, cb_tick_labels, clevels)

function plot_heatmap(x, y, values, i, ylab, max_clr, clr_points, clim, use_percent, n_pop, cb_ticks, cb_tick_labels, clevels)

    % Create figure
    figure1 = figure(i);
   
    % Convert to %
    x = x'*100;
    
    % Convert number of residents to percent population
    if use_percent
        y = y/n_pop*100;
    end
    
    % First check the acutal limits, then restrict ColorLimits - otherwise
    % biased
%     h = heatmap(x,y,values,'Title', ylab, 'CellLabelColor','none');
    h = heatmap(x,y,values,'Title', ylab, 'CellLabelColor','none', 'ColorLimits', [0,clim], ...
        'InnerPosition',[0.177438591597373 0.191765170084011 0.6530877241921 0.708234829915988]);
        
  
    % Create custom colormap
    colorMap = [linspace(1,max_clr(1),clr_points)', ...
                linspace(1,max_clr(2),clr_points)',...
                linspace(1,max_clr(3),clr_points)'];
    colorMap = colorMap(2:end,:);
            
%     color_level_1 = (max_clr(1)-1)/length(cb_ticks);
%     color_level_2 = (max_clr(2)-1)/length(cb_ticks);
%     color_level_3 = (max_clr(3)-1)/length(cb_ticks);
%     n_sub = 10;
%     for ic = 1:length(cb_ticks)
%          colorMap(1+n_sub*(ic-1):n_sub*ic, 1) = linspace(1+color_level_1*(ic-1), 1+color_level_1*(ic), n_sub);
%          colorMap(1+n_sub*(ic-1):n_sub*ic, 2) = linspace(1+color_level_2*(ic-1), 1+color_level_2*(ic), n_sub);
%          colorMap(1+n_sub*(ic-1):n_sub*ic, 3) = linspace(1+color_level_3*(ic-1), 1+color_level_3*(ic), n_sub);
%     end

    colormap(h, colorMap)
    h.GridVisible = 'off';

%     % Labels
%     h.XLabel = 'Reopening rate, %/day';
%     if use_percent
%         h.YLabel = ['Vaccination rate,', newline, '%(population)/day'];
%     else
%         h.YLabel = 'Vaccination rate, people/day';
%     end
  
    % This should be preceeded by examining the range
    % Customize y ticks if using percents
    if use_percent
        % First check
        CustomYLabels = {'0.01', '0.02', '0.05', '0.1', '0.2', '0.5', '1', '2', '5'};
        h.YDisplayLabels = CustomYLabels;
    else
        h.YDisplayLabels = {'5', '50', '100', '250', '500', '750', '1,000', ...
                           '2,500', '5,000', '8,000'};
    end
    h.YDisplayData = flipud(h.YDisplayData); 
                    
    % Adjust colobar 
    hCB=h.NodeChildren(2);         
    hCB.Ticks = cb_ticks;
    hCB.TickLabels = cb_tick_labels;
      
    % Fonts
    h.FontName = 'Arial';
    h.FontSize = 28;
   
    % Size
    set(gcf,'Position',[200 500 950 750])
    
        
  
    % Contours
    hAx = axes('Position',h.Position,'Color','none'); 
    hold on
    [M,c] = contour(hAx,round(values),clevels, 'ShowText','on','LevelList',clevels);
    clabel(M,c,'FontSize',22,'Color','k', 'FontName', 'Arial');
%     
%     for itn = 1:length(texth)
%         textstr=get(texth(itn),'String');
%         textnum=str2num(textstr);
%         textstrnew=num2str(textnum,'%,.0f');
%         set(texth(n),'String',teststrnew);
%     end
%     c.YAxis.TickLabelFormat = '%,.0f';
    % To see what is available
%      [M,c] = contour(hAx,round(values),5, 'ShowText','on')
    c.LineWidth = 2.5;
    c.LineColor = 'k';
    axis(hAx,'tight');
    hold(hAx,'off');
    % Set the remaining axes properties
    set(hAx,'Color','none','XTick',zeros(1,0),'YTick',zeros(1,0));


end
