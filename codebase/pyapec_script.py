import numpy
import pyatomdb

cie = pyatomdb.spectrum.CIESession()

def pyapec(engs, params, flux):
    ebins = numpy.array(engs)

    elements = []
    for i in range(1, 31):
        elements.append(i)

    eloffset = 0
    abund = None

    if len(params) == 4:
        abund = numpy.array(params[1])
        elements = [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30]
        eloffset = 2
    elif len(params) == 17:
        elements = [1, 2, 6, 7, 8, 10, 12, 13, 14, 16, 18, 20, 26, 28]
        abund = params[1:15]
        eloffset = 15
    elif len(params) == 33:
        elements = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30]
        abund = params[1:31]
        eloffset = 31

    if abund is None:
        raise ValueError("Invalid params")

    redshift = float(params[eloffset + 0])
    cie.set_response(ebins * (1.0 + redshift), raw=True)
    cie.set_abund(elements, abund)
    cie.set_eebrems(False)

    spec = cie.return_spectrum(params[0], log_interp=False)
    flux[:] = spec * 1e14

    return flux
