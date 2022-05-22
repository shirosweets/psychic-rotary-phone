import plotly.express as px

from measurement import Measurement

def graph_measurements(measurements : list[Measurement]):
    graph_useful_load_curve(measurements)
    graph_delay_curve(measurements)
    graph_network_overload(measurements)


def graph_useful_load_curve(measurements : list[Measurement]):
    """
    Eje x: Carga ofrecida: Paquetes por segundo que se enviaron
    Eje y: Carga util: Paquetes por segundo que se recibieron
    """
    data = [m.to_dictionary() for m in measurements]

    fig = px.line(
        data, x=Measurement.GENRATE_TAG,
        y=Measurement.DELIVERED_TAG,
        color=Measurement.CASE_TAG,
        markers = True
    )
    fig.show()

    # x_axis = list(map(generated_pkg_per_s, measurements))
    # x_label = "Carga Ofrecida [pkg/s]"
    # y_axis = list(map(received_pkg_per_s, measurements))
    # y_label = "Carga Util [pkg/s]"
    # x_axis.insert(0, 0.0)
    # y_axis.insert(0, 0.0)

    # dataset = [ (x_axis[i], y_axis[i]) for i in range(len(x_axis))]
    # dataset.insert(0, (0.0,0.0))
    # for e in dataset:
    #     print(e)
    # fig = px.scatter(x=x_axis, y=y_axis)
    # fig.update_layout(xaxis_title=x_label, yaxis_title=y_label)
    # fig.show()


def graph_network_overload(measurements):
    pass



def graph_delay_curve(measurements : list[Measurement]):
    pass

