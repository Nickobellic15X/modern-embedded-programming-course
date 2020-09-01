/*.$file${.::qmsmtst.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: qmsmtst.qm
* File:  ${.::qmsmtst.h}
*
* This code has been generated by QM 5.0.1 <www.state-machine.com/qm/>.
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This code is covered by the following QP license:
* License #   : QPC-EVAL
* Issued to   : Institution or an individual evaluating the QP/C framework
* Framework(s): qpc
* Support ends: 2020-12-31
* Product(s)  :
* This license is available only for evaluation purposes and
* the generated code is still licensed under the terms of GPL.
* Please submit request for extension of the evaluaion period at:
* <www.state-machine.com/licensing/#RequestForm>
*/
/*.$endhead${.::qmsmtst.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifndef QMSMTST_H
#define QMSMTST_H

enum QMsmTstSignals {
    A_SIG = Q_USER_SIG,
    B_SIG,
    C_SIG,
    D_SIG,
    E_SIG,
    F_SIG,
    G_SIG,
    H_SIG,
    I_SIG,
    TERMINATE_SIG,
    IGNORE_SIG,
    MAX_SIG
};

extern QHsm * const the_sm; /* opaque pointer to the test SM */

/*.$declare${SMs::QMsmTst_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::QMsmTst_ctor} ....................................................*/
void QMsmTst_ctor(void);
/*.$enddecl${SMs::QMsmTst_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/* BSP functions to dispaly a message and exit */
void BSP_display(char const *msg);
void BSP_exit(void);

#endif /* QMSMTST_H */