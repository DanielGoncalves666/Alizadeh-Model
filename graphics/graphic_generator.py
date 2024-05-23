import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors
import argparse
import time
import math

def mean(values):
    return sum(values) / len(values)

def set_colormap():
    hmap = matplotlib.colors.LinearSegmentedColormap.from_list("heatmap", ["darkblue","blue","cyan","limegreen","yellow","orange","red","darkred"])
    hmap.set_over("red") # values above MAX_VALUE receive red color
    hmap.set_under("black") # values below MIN_VALUE receive black color
    
    return hmap

def process_data_alizadeh_evac_time(filename):
    # This function will assume that the format of the data inside 'filename' follows the sintax used to write the results of discrete time simulations in the alizadeh.sh program.
    
    mean_vector = []
    min_value_global = math.inf
    max_value_global = -math.inf
    
    try:
        with open(filename) as file:
            for _ in range(3):
                file.readline() # ignore the lines that doesn't contain simulation data on the top of the file 
                
            while True:
                line = file.readline()
                
                if line == "":
                    break # EOF reached
                
                ignore_min_max = False
                raw_values = line.strip("\n ").split(" ")
                if raw_values[0] == "#1":
                    ignore_min_max = True
                    raw_values = raw_values[1:]
                    # lines beggining with #1 indicate a set of simulations that were done on a room with only one exit, because when the combination of two exits was done they coincided on the
                    # same place.
                    
                values = list(map(int, raw_values))
                
                if not ignore_min_max:
                    min_current = min(values)
                    max_current = max(values)
                    if min_current != -1 and min_current < min_value_global:
                        min_value_global = min_current
                    
                    if max_current != -1 and max_current > max_value_global:
                        max_value_global = max_current
                
                # the values -1 in the data refer to pair of exits that are not valid (at least one of them is inaccessible) and therefore should be ignored in the min/max values
                
                mean_value = mean(values)
                mean_vector.append(mean_value)
    except FileNotFoundError:
        print(f"File {filename} not found.")
        exit()
            
    # since alizadeh example 1 is a combination of two exits out of 81, if exits_number isn't at least a integer then there are not enough lines in the file
    exits_number = math.sqrt(len(mean_vector))
    exits_number_integer = int(exits_number)
    if abs(exits_number - exits_number_integer) > 1e-8:
        print(f"Not enough data lines in {filename}")
        exit()
        
    return (np.array(mean_vector).reshape(exits_number_integer, exits_number_integer), (min_value_global, max_value_global))
        
def process_data_alizadeh_ped_distribution(filename):
    #This function will assume that for every line there are only one value
    
    value_vector = []
    min_value_global = math.inf
    max_value_global = -math.inf
    
    try:
        with open(filename) as file:
            for _ in range(3):
                file.readline() # ignore the lines that doesn't contain simulation data on the top of the file 
                
            while True:
                line = file.readline()
                
                if line == "":
                    break # EOF reached
                
                ignore_min_max = False
                raw_values = line.strip("\n ").split(" ")
                if raw_values[0] == "#1":
                    ignore_min_max = True
                    raw_values = raw_values[1:]
                    # lines beggining with #1 indicate a set of simulations that were done on a room with only one exit, because when the combination of two exits was done they coincided on the
                    # same place.
                    
                value = float(raw_values[0])
                
                if not ignore_min_max:
                    if value != -1 and value < min_value_global:
                        min_value_global = value
                    
                    if value != -1 and value > max_value_global:
                        max_value_global = value
                
                # the values -1 in the data refer to pair of exits that are not valid (at least one of them is inaccessible) and therefore should be ignored in the min/max values
                
                value_vector.append(value)
    except FileNotFoundError:
        print(f"File {filename} not found.")
        exit()   
        
    # since alizadeh example 1 is a combination of two exits out of 81, if exits_number isn't at least a integer then there are not enough lines in the file
    exits_number = math.sqrt(len(value_vector))
    exits_number_integer = int(exits_number)
    if abs(exits_number - exits_number_integer) > 1e-8:
        print(f"Not enough data lines in {filename}")
        exit()
        
    return (np.array(value_vector).reshape(exits_number_integer, exits_number_integer), (min_value_global, max_value_global))

def process_data_alizadeh_relation_alfa_timesteps(filename):
    
    x_axis = []
    y_axis = []
    
    try:
        with open(filename) as file:
            lines = file.readlines()
            
            x_axis = lines[0].strip("\n ").split(" ")

            for line in lines[1:]:
                y_axis.append(mean( [float(x) for x in line.strip("\n ").split(" ")]))
    except FileNotFoundError:
        print(f"File {filename} not found.")
        exit()
        
    if len(x_axis) != len(y_axis):
        print(f"The number of elements in the x axis ({len(x_axis)}) is different from the number os elements in the y axix ({len(y_axis)}).")
        exit()
        
    return (x_axis, y_axis)
               
def plot_alizadeh_evac_time(data_matrix, min_max_values, output_file, labels, g_type):
    min_value, max_value = min_max_values
    levels = np.arange(min_value,max_value, (max_value - min_value) // 8)
    
    plot_heatmap(data_matrix, min_value, max_value, levels, output_file, labels, g_type)

def plot_alizadeh_ped_distribution(data_matrix, min_max_values, output_file, labels, g_type):
    min_value, max_value = min_max_values
    levels = np.linspace(min_value,max_value, 8, endpoint=False)
    
    plot_heatmap(data_matrix, min_value, max_value, levels, output_file, labels,g_type)

def plot_heatmap(data_matrix, min_value, max_value, levels, output_file, labels, g_type):
    fig = plt.figure()
        
    if g_type == "contours":
        plt.contour(data_matrix, vmin=min_value, vmax=max_value, levels=levels, colors="black", origin="lower", extend="both") # generate contour lines
        plt.contourf(data_matrix, vmin=min_value, vmax=max_value, levels=levels, colors=["darkblue","blue","cyan","limegreen","yellow","orange","red","darkred"], origin="lower", extend="both") # generate contour lines
    else:
        plt.imshow(data_matrix, vmin=min_value, vmax=max_value, cmap=set_colormap(), origin="lower") # generate the heatmap
        
    plt.colorbar() # add the colorbar
        
    plt.title(labels[0])
    plt.xlabel(labels[1])
    plt.ylabel(labels[2])
    
    plt.show() # show the graphic
    
    fig.savefig(f"out/{output_file}")
    
def plot_alizadeh_relation_alfa_timesteps(x_axis, y_axis, output_file, labels):
    fig = plt.figure()
    plt.plot(x_axis, y_axis, "o-k")

    plt.title(labels[0])
    plt.xlabel(labels[1])
    plt.ylabel(labels[2])
    
    plt.show() # show the graphic
    
    fig.savefig(f"out/{output_file}")

def creating_arg_parser():
    # ArgumentParser will contain all information about the command line interface
    parser = argparse.ArgumentParser(description='Graphic Generator')
    
    # add_argument adds new arguments or options that can be inserted by command line
    parser.add_argument('-i', required=True, nargs=1, help="Filename that contains the data from which the graphic will be generated.")
    possible_graphics = ["alizadeh_evac_time", "alizadeh_ped_distribution", "alizadeh_relation_alfa_timesteps"]
    parser.add_argument('-g','--graphic', choices=possible_graphics, required=True, nargs=1, help="Specifies which graphic should be generated.")
    parser.add_argument('-o','--out', nargs="?", default="", help="Filename on which the graphic should be saved.")
    parser.add_argument('-t','--title', nargs=1, help="The title of the generated graphic.")
    parser.add_argument('-x', '--xlabel', nargs=1, help="X axis label")
    parser.add_argument('-y', '--ylabel', nargs=1, help="Y axis label")
    parser.add_argument('--type', choices=["heatmap", "contours"], nargs=1, help="Indicates which type of graphic should be generated (contours by default).")
    
    return parser

def generate_graphic(input_file, choice, output_file, labels,g_type):
    
    if choice == "alizadeh_evac_time":
        (data_matrix, min_max_values) = process_data_alizadeh_evac_time(input_file)
        plot_alizadeh_evac_time(data_matrix, min_max_values, output_file, labels, g_type)
    elif choice == "alizadeh_ped_distribution":
        (data_matrix, min_max_values) = process_data_alizadeh_ped_distribution(input_file)
        plot_alizadeh_ped_distribution(data_matrix, min_max_values, output_file, labels, g_type)
    elif choice == "alizadeh_relation_alfa_timesteps":
        (x_axis, y_axis) = process_data_alizadeh_relation_alfa_timesteps(input_file)
        plot_alizadeh_relation_alfa_timesteps(x_axis, y_axis, output_file, labels)
    else:
        print("Invalid graphic.")
        exit()

if __name__ == "__main__":
    command_line = creating_arg_parser().parse_args()
    
    input_file = command_line.i[0]
    choice = command_line.graphic[0]
    output_file = command_line.out if command_line.out != "" else f"{choice}_{time.strftime('%Y-%m-%d_%H:%M:%S')}.png"
    
    labels = [command_line.title[0] if command_line.title is not None else "",
              command_line.xlabel[0] if command_line.xlabel is not None else "",
              command_line.ylabel[0] if command_line.ylabel is not None else ""]
    
    g_type = command_line.type[0] if command_line.type is not None else "contours"
    
    generate_graphic(input_file, choice, output_file,labels, g_type)
